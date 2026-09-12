#include "bsp/scene_traffic_groups.hpp"

#include <cctype>
#include <cstdio>
#include <utility>

// Evidence: docs/SCENE_TRAFFIC_BLOCK.md, docs/SCENE_BROWSER_GROUPS.md,
// docs/SCENE_PROPERTY_LIBRARY.md. Report: reports/scene_traffic_groups.json.
//
// Every routine below is a sequence over an injected host, one host method per
// native call site. None of it is binary-compatible with the image: the traffic
// record is a C++ struct, not the 4Ch native layout, and the property bag is
// the ScenePropertyBlock of bsp/scene_file.hpp rather than the 114h object.
namespace bsp {
namespace {

// 00438E10, the case-insensitive compare every keyword test goes through.
// src/scene_file.cpp has its own copy in its own anonymous namespace; this one
// is local to this translation unit for the same reason.
bool equal_ci(const std::string& a, const char* b) noexcept
{
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

std::string expected_error(int line, const char* what, const std::string& got)
{
    char buffer[192];
    std::snprintf(buffer, sizeof buffer, "line %d: expected %s, got \"%.80s\"", line, what,
        got.c_str());
    return std::string(buffer);
}

bool at_token(SceneLexer& lexer, const char* text)
{
    const SceneToken& t = lexer.peek();
    return !t.is_end() && equal_ci(t.text, text);
}

// 008D9930 BSP_SceneTokenizer_ExpectToken: consume only on a match, and leave
// the token in place otherwise. docs/SCENE_FILE_READER.md calls that rule the
// whole error-recovery story.
bool expect_token(SceneLexer& lexer, const char* text, std::string& error)
{
    const SceneToken& t = lexer.peek();
    if (!t.is_end() && equal_ci(t.text, text)) {
        lexer.next();
        return true;
    }
    if (error.empty()) {
        error = expected_error(t.line, text, t.text);
    }
    return false;
}

// 008D9980 BSP_SceneTokenizer_ReadToken.
std::string read_any_token(SceneLexer& lexer, std::string& error)
{
    const SceneToken& t = lexer.peek();
    if (t.is_end()) {
        if (error.empty()) {
            error = expected_error(t.line, "a token", t.text);
        }
        return std::string();
    }
    return lexer.next().text;
}

// 008D9AD0 BSP_SceneTokenizer_ReadInt.
std::int32_t read_int_token(SceneLexer& lexer, std::string& error)
{
    const std::string text = read_any_token(lexer, error);
    std::int32_t value = 0;
    if (!scene_scan_int(text, value) && error.empty()) {
        error = expected_error(lexer.line(), "an integer", text);
    }
    return value;
}

// 0049CF80 reads the 38h property record's type code at +04h and takes the
// float at +0Ch directly when it is 1, otherwise converts the same dword with
// CVTSI2SS. This reconstruction has the authored type letter rather than the
// resolved code, so it tests the letter the property-bag doc pairs with code 1.
bool letter_is_float(const SceneProperty& prop) noexcept
{
    return equal_ci(prop.type_letter, "F");
}

float property_as_float(const SceneProperty& prop, float fallback) noexcept
{
    if (prop.values.empty()) {
        return fallback;
    }
    if (letter_is_float(prop)) {
        float value = 0.0f;
        return scene_scan_float(prop.values.front(), value) ? value : fallback;
    }
    std::int32_t whole = 0;
    if (scene_scan_int(prop.values.front(), whole)) {
        return static_cast<float>(whole);
    }
    return fallback;
}

std::int32_t property_as_int(const SceneProperty& prop, std::int32_t fallback) noexcept
{
    if (prop.values.empty()) {
        return fallback;
    }
    std::int32_t value = 0;
    return scene_scan_int(prop.values.front(), value) ? value : fallback;
}

// The five integer and boolean fields take +0Ch raw with no type check, so a
// `B true` value has to be read as the boolean it is authored as.
bool property_as_bool(const SceneProperty& prop, bool fallback) noexcept
{
    if (prop.values.empty()) {
        return fallback;
    }
    const std::string& text = prop.values.front();
    if (equal_ci(text, "true")) {
        return true;
    }
    if (equal_ci(text, "false")) {
        return false;
    }
    std::int32_t value = 0;
    return scene_scan_int(text, value) ? value != 0 : fallback;
}

void apply_float(const ScenePropertyBlock& block, const char* key, float fallback, float& out)
{
    // 0048E9F0 probe, then 008F2260 BSP_ScenePropertyBag_Find. One probe and one
    // fetch per key, which is what the native does even though the probe alone
    // would answer the question.
    const SceneProperty* prop = block.find(key);
    out = (prop == nullptr) ? fallback : property_as_float(*prop, fallback);
}

void apply_int(const ScenePropertyBlock& block, const char* key, std::int32_t fallback,
    std::int32_t& out)
{
    const SceneProperty* prop = block.find(key);
    out = (prop == nullptr) ? fallback : property_as_int(*prop, fallback);
}

const ScenePropertyBlock* find_sub_block(const ScenePropertyBlock& block, const char* name)
{
    for (const auto& entry : block.blocks) {
        if (equal_ci(entry.first, name)) {
            return &entry.second;
        }
    }
    return nullptr;
}

} // namespace

// ---------------------------------------------------------------------------
// 0049CF80 BSP_TrafficRecord_ApplyProperties
// ---------------------------------------------------------------------------
void apply_traffic_properties(const ScenePropertyBlock& block, SceneTrafficHost& host,
    TrafficRecord& record)
{
    // Thirteen keys, in the order 0049CF80 reads them. The two point indices are
    // the one place the applier disagrees with the constructor: it writes 0 for
    // an absent key where 00951220 wrote -1.
    apply_int(block, kTrafficKeyStartPoint, TrafficRecordDefaults::kAppliedStartPoint,
        record.start_point);
    apply_int(block, kTrafficKeyEndPoint, TrafficRecordDefaults::kAppliedEndPoint,
        record.end_point);

    const SceneProperty* cyclic = block.find(kTrafficKeyCyclic);
    record.cyclic = (cyclic == nullptr) ? false : property_as_bool(*cyclic, false);

    apply_int(block, kTrafficKeyRowCount, 0, record.row_count);
    apply_int(block, kTrafficKeyColumns, TrafficRecordDefaults::kColumns, record.columns);
    apply_float(block, kTrafficKeyRowGap, TrafficRecordDefaults::kRowGap, record.row_gap);
    apply_float(block, kTrafficKeyColumnGap, TrafficRecordDefaults::kColumnGap,
        record.column_gap);
    apply_float(block, kTrafficKeyHitPoints, 0.0f, record.hit_points);
    apply_float(block, kTrafficKeySpeed, TrafficRecordDefaults::kSpeed, record.speed);
    apply_float(block, kTrafficKeyRandomFactor, 0.0f, record.random_factor);
    apply_float(block, kTrafficKeyRowDeviation, 0.0f, record.row_deviation);
    apply_float(block, kTrafficKeyColumnDeviation, 0.0f, record.column_deviation);

    // `templates` is fetched through the same 008F2260 and then required to have
    // type code 6, the sub-block code; anything else is treated as absent.
    const ScenePropertyBlock* templates = find_sub_block(block, kTrafficKeyTemplates);
    if (templates == nullptr) {
        return;
    }

    for (const SceneProperty& child : templates->values) {
        TrafficTemplateWeight entry;
        entry.name = child.key;
        entry.weight = property_as_float(child, 0.0f);

        // 0049D25A LandVehicleClasses first, 0049D2AC SoldierTypes on a miss.
        if (!host.resolve_template_class(entry.name, entry.land_vehicle_class, entry.class_id)) {
            // The native has no miss arm: 0048E840 returns whatever the table
            // holds for an absent symbol. A reconstruction that invented an id
            // would be worse than one that drops the row, so it drops it and the
            // divergence is recorded in docs/SCENE_TRAFFIC_BLOCK.md.
            continue;
        }
        if (entry.land_vehicle_class) {
            host.ensure_vehicle_class(entry.class_id);   // 0049D291, 00964790
        } else {
            host.ensure_soldier_type(entry.class_id);    // 0049D311, 004B1400
        }
        // 0049D364: 00499030 on record+34h returns the float* the weight lands in.
        record.templates.push_back(std::move(entry));
    }
}

// ---------------------------------------------------------------------------
// 009512F0 BSP_SceneFile_ReadTrafficItem
// ---------------------------------------------------------------------------
bool read_traffic_item(SceneLexer& lexer, SceneTrafficHost& host, TrafficRecord& out,
    std::string& item_name, std::string& error)
{
    out = TrafficRecord();

    if (!expect_token(lexer, kTrafficItemKeyword, error)) {
        return false;
    }
    // Parsed, passed to 00951220 as argument 1, and never read there: the
    // 51-instruction body touches only the argument-2 slot.
    item_name = read_any_token(lexer, error);
    if (!expect_token(lexer, "{", error)) {
        return false;
    }
    if (!expect_token(lexer, kTrafficPathKeyword, error)) {
        return false;
    }
    if (!expect_token(lexer, "=", error)) {
        return false;
    }
    out.path_entity_name = read_any_token(lexer, error);

    // 00951369/0095136D: the lookup runs only for a non-null, non-empty name.
    if (!out.path_entity_name.empty()) {
        out.path_entity_resolved = host.resolve_path_entity(out.path_entity_name);
    }

    // 009513A4: the optional `properties` section.
    if (at_token(lexer, "properties")) {
        if (!expect_token(lexer, "properties", error)) {
            return false;
        }
        std::vector<std::string> parse_errors;
        const ScenePropertyBlock block = parse_scene_property_block_008f5a00(lexer, parse_errors);
        if (!parse_errors.empty() && error.empty()) {
            error = parse_errors.front();
        }
        apply_traffic_properties(block, host, out);
    }

    if (!expect_token(lexer, "}", error)) {
        return false;
    }

    // Ordering divergence, deliberate: the native creates the record at
    // 00951396 and applies the properties to it at 009513E8, so a bag that
    // failed mid-parse would leave a half-applied record in the vector. This
    // hands the host a finished record instead. The end state is the same.
    host.add_traffic_record(item_name, out);
    return true;
}

// ---------------------------------------------------------------------------
// 009514B0 BSP_SceneFile_ReadTrafficBlock
// ---------------------------------------------------------------------------
SceneTrafficReadResult read_traffic_block(SceneLexer& lexer, SceneTrafficHost& host)
{
    SceneTrafficReadResult result;

    host.clear_traffic_records();   // 009514B5

    std::string error;
    if (!expect_token(lexer, kTrafficBlockKeyword, error)) {
        result.error = error;
        return result;
    }
    if (!expect_token(lexer, "{", error)) {
        result.error = error;
        return result;
    }

    // 009514D6 MOV EDI,1, then the brace counter. Only `item` is a production;
    // every other token is counted and consumed, so an unknown child block is
    // skipped rather than rejected.
    int depth = 1;
    while (depth > 0) {
        if (lexer.at_end()) {
            result.error = error.empty() ? std::string("unterminated traffic block") : error;
            return result;
        }
        if (at_token(lexer, kTrafficItemKeyword)) {
            TrafficRecord record;
            std::string item_name;
            if (!read_traffic_item(lexer, host, record, item_name, error)) {
                result.error = error;
                return result;
            }
            result.records.push_back(std::move(record));
            // 009514FF jumps past the consume: the item reader consumed its own
            // closing brace.
            continue;
        }
        if (at_token(lexer, "{")) {
            ++depth;                 // 00951518
        } else if (at_token(lexer, "}")) {
            --depth;                 // 00951534
        } else {
            ++result.skipped_tokens;
        }
        lexer.next();                // 00951539
    }

    host.commit_traffic_records();   // 0095154D
    result.ok = error.empty();
    result.error = error;
    return result;
}

// ---------------------------------------------------------------------------
// 0095C640 BSP_VehicleClass_RegisterPreload
// ---------------------------------------------------------------------------
int register_vehicle_class_preload(std::int32_t class_id, VehicleClassPreloadHost& host)
{
    // registry+10h+id*4, the forward index map, identity unless remapped.
    const std::int32_t index = host.map_class_index(class_id);
    if (index == kVehicleClassNone) {
        return 0;                    // 0095C66E
    }

    std::string type;
    if (!host.read_class_type(index, type)) {
        type.clear();
    }

    int appended = 0;

    // VehicleClass[index].LandingShip, default 0; any non-zero value is a class
    // of its own and is appended before the class itself.
    const std::int32_t landing_ship = host.read_landing_ship_class(index);
    if (landing_ship != 0) {
        host.append_census(landing_ship);
        ++appended;
    }

    host.append_census(index);
    ++appended;

    bool carrier = false;
    for (std::size_t i = 0; i < kCatapultCarrierTypeCount; ++i) {
        if (equal_ci(type, kCatapultCarrierTypes[i])) {
            carrier = true;
            break;
        }
    }
    if (!carrier) {
        return appended;
    }

    // "VehicleClass." + index + ".Catapult.LaunchedClass", default -1.
    const std::int32_t launched = host.read_catapult_launched_class(index);
    if (launched >= 0) {
        host.append_census(launched);
        ++appended;
    }
    return appended;
}

// ---------------------------------------------------------------------------
// 0095CA10 BSP_SceneFile_DrainStockClassQueue
// ---------------------------------------------------------------------------
int drain_stock_class_queue(VehicleClassPreloadHost& host)
{
    int drained = 0;
    std::int32_t class_index = 0;
    while (host.pop_stock_queue(class_index)) {
        register_vehicle_class_preload(class_index, host);
        ++drained;
    }
    return drained;
}

// ---------------------------------------------------------------------------
// 00469E40 BSP_SceneFile_ReadBrowserGroupsBlock and 004694F0
// ---------------------------------------------------------------------------
SceneBrowserGroupsReadResult read_browser_groups_block(SceneLexer& lexer,
    SceneBrowserGroupsHost& host)
{
    SceneBrowserGroupsReadResult result;
    std::string error;

    if (!expect_token(lexer, kBrowserGroupsBlockKeyword, error)) {
        result.error = error;
        return result;
    }
    if (!expect_token(lexer, "{", error)) {
        result.error = error;
        return result;
    }

    // 00469E62: peek, and repeat only while the token is `BrowserGroup`. There
    // is no brace counter here; an unexpected token ends the loop and the
    // closing `}` expect reports it.
    while (at_token(lexer, kBrowserGroupKeyword)) {
        SceneBrowserGroupEntry entry;
        // 004694F0: eleven ExpectToken calls in fixed order, no dispatch.
        if (!expect_token(lexer, kBrowserGroupKeyword, error)
            || !expect_token(lexer, "{", error)
            || !expect_token(lexer, kBrowserGroupNameKey, error)
            || !expect_token(lexer, "=", error)) {
            result.error = error;
            return result;
        }
        entry.group_name = read_any_token(lexer, error);
        if (!expect_token(lexer, kBrowserGroupIdKey, error)
            || !expect_token(lexer, "=", error)) {
            result.error = error;
            return result;
        }
        // The native writes this into the caller's dead argument slot and never
        // reads it back; it is kept here so the grammar can be checked.
        entry.group_id = read_int_token(lexer, error);
        if (!expect_token(lexer, kBrowserGroupParentKey, error)
            || !expect_token(lexer, "=", error)) {
            result.error = error;
            return result;
        }
        entry.parent = read_any_token(lexer, error);
        if (!expect_token(lexer, "}", error)) {
            result.error = error;
            return result;
        }
        host.observe_browser_group(entry);
        result.entries.push_back(std::move(entry));
    }

    if (!expect_token(lexer, "}", error)) {
        result.error = error;
        return result;
    }
    result.ok = error.empty();
    result.error = error;
    return result;
}

// ---------------------------------------------------------------------------
// 008F67B0 CPropTreeLibrary::Load (recovered symbol)
// ---------------------------------------------------------------------------
bool load_prop_library_file(const std::string& path, PropLibraryHost& host,
    PropLibraryDocument& document, std::string& error)
{
    std::string text;
    if (!host.read_library_file(path, text)) {
        error = "cannot read " + path;
        return false;
    }

    // 008F68A2/008F68DA: a tokenizer of its own, over a delimiter set that is
    // the scene set without the comma.
    SceneLexer lexer(std::move(text), kPropLibraryDelimiters);

    while (!lexer.at_end()) {
        if (at_token(lexer, kPropLibraryEnumKeyword)) {
            lexer.next();                                   // 008F6953
            PropLibraryEnumTable table;
            table.name = read_any_token(lexer, error);      // 008F695A
            table.created = !host.has_enum_table(table.name); // 008F697C
            if (!expect_token(lexer, "{", error)) {         // 008F698A
                return false;
            }
            // 008F69FF 008F31A0 parses the body. That callee was not opened, so
            // the body is handed over as text rather than decoded here.
            std::string body;
            int depth = 1;
            while (depth > 0 && !lexer.at_end()) {
                const SceneToken token = lexer.next();
                if (token.text == "{") {
                    ++depth;
                } else if (token.text == "}") {
                    if (--depth == 0) {
                        break;
                    }
                }
                if (!body.empty()) {
                    body.push_back(' ');
                }
                body += token.text;
            }
            if (depth > 0) {
                error = "unterminated enum " + table.name;
                return false;
            }
            host.parse_enum_body(table.name, body);
            if (table.created) {
                host.register_enum_table(table);            // 008F6A18
            }
            document.enums.push_back(std::move(table));
            continue;
        }

        if (at_token(lexer, kPropLibraryPropertiesKeyword)) {
            lexer.next();                                   // 008F6A3D
            PropLibraryGroup group;
            group.name = read_any_token(lexer, error);      // 008F6A44
            group.created = !host.has_group(group.name);    // 008F6A69

            // The parenthesised parent. 008F5A00 is what resolves it, against
            // the registry this loader passes as its third argument, which is
            // the library itself; it is read out here so the document records it.
            if (at_token(lexer, "(")) {
                lexer.next();
                while (!lexer.at_end() && !at_token(lexer, ")")) {
                    const std::string name = read_any_token(lexer, error);
                    if (group.parent.empty()) {
                        group.parent = name;
                    }
                }
                if (!expect_token(lexer, ")", error)) {
                    return false;
                }
            }

            std::vector<std::string> parse_errors;
            group.body = parse_scene_property_block_008f5a00(lexer, parse_errors);
            if (!parse_errors.empty() && error.empty()) {
                error = parse_errors.front();
            }
            if (group.created) {
                host.register_group(group);                 // 008F6B42
            }
            document.groups.push_back(std::move(group));
            continue;
        }

        // 008F67B0 consumes nothing for a token that is neither keyword, so the
        // native would spin. This consumes it, and the divergence is recorded in
        // docs/SCENE_PROPERTY_LIBRARY.md open question 3. No shipped file hits it.
        lexer.next();
    }

    return error.empty();
}

// ---------------------------------------------------------------------------
// 008F7100 then 008F6FC0 twice
// ---------------------------------------------------------------------------
PropLibraryLoadResult load_prop_library(const std::string& folder, PropLibraryHost& host)
{
    PropLibraryLoadResult result;

    // 008F710E then 008F711B: .enums first, so every .props file's enum-typed
    // defaults resolve against tables that already exist.
    const char* extensions[] = { kPropLibraryEnumExtension, kPropLibraryPropsExtension };
    for (const char* extension : extensions) {
        const std::vector<std::string> files = host.enumerate_library_files(folder, extension);
        for (const std::string& file : files) {
            std::string error;
            if (!load_prop_library_file(file, host, result.document, error)
                && result.error.empty()) {
                result.error = error;
            }
        }
    }

    result.ok = result.error.empty();
    return result;
}

} // namespace bsp
