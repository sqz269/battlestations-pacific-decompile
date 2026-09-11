// Scene property bag rules. Evidence: docs/SCENE_PROPERTY_BAG.md.
// Every constant here is quoted from a native store or compare; nothing is
// invented to make a call site link.
#include "bsp/scene_property_bag.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>

namespace bsp {
namespace {

bool equals_no_case(const std::string& a, const std::string& b) noexcept {
    // 0043B8B0 compares the stored key with __stricmp after a hash hit.
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        const unsigned char ca = static_cast<unsigned char>(a[i]);
        const unsigned char cb = static_cast<unsigned char>(b[i]);
        if (std::tolower(ca) != std::tolower(cb)) {
            return false;
        }
    }
    return true;
}

bool parse_float(const std::string& token, float& out) noexcept {
    // 008D8F40 accepts a token when sscanf("%f") consumes exactly one field.
    char* end = nullptr;
    const double value = std::strtod(token.c_str(), &end);
    if (end == token.c_str()) {
        return false;
    }
    out = static_cast<float>(value);
    return true;
}

bool parse_int(const std::string& token, std::int32_t& out) noexcept {
    char* end = nullptr;
    const long value = std::strtol(token.c_str(), &end, 10);
    if (end == token.c_str()) {
        return false;
    }
    out = static_cast<std::int32_t>(value);
    return true;
}

}  // namespace

const char* scene_property_type_letter(ScenePropertyType type) noexcept {
    switch (type) {
        case ScenePropertyType::Int: return "I";                // 00D16528
        case ScenePropertyType::Float: return "F";              // 00CFAD04
        case ScenePropertyType::String: return "S";             // 00D16524
        case ScenePropertyType::Bool: return "B";               // 00CE60E4
        case ScenePropertyType::Enum: return "E";               // 00D162D4
        case ScenePropertyType::Reference: return "R";          // 00CE60EC
        case ScenePropertyType::SubBag: return "";              // no letter
        case ScenePropertyType::Vector3: return "V3";           // 00D16520
        case ScenePropertyType::ByteArray: return "BIN";        // 00D1651C
        case ScenePropertyType::FloatArray: return "FA";        // 00D16518
        case ScenePropertyType::IntArray: return "IA";          // 00D16514
        case ScenePropertyType::Vector3Array: return "V3A";     // 00D16510
    }
    return "";
}

const char* scene_reference_kind_letter(SceneReferenceKind kind) noexcept {
    switch (kind) {
        case SceneReferenceKind::Any: return "R";                   // 00CE60EC
        case SceneReferenceKind::Land: return "RLand";              // 00D1637C
        case SceneReferenceKind::Fort: return "RFort";              // 00D16384
        case SceneReferenceKind::Ship: return "RShip";              // 00D1636C
        case SceneReferenceKind::Plane: return "RPlane";            // 00D16374
        case SceneReferenceKind::PlaneOrShip: return "RPlnShp";     // 00D16364
        case SceneReferenceKind::LandPlaneOrShip: return "RLPlnShp";// 00D16358
        case SceneReferenceKind::Path: return "RPath";              // 00D1638C
    }
    return "R";
}

bool scene_property_type_for_letter(const std::string& letter,
                                    ScenePropertyType& type,
                                    SceneReferenceKind& kind) noexcept {
    kind = SceneReferenceKind::Any;
    // Reference letters first: 008F5FED..008F60A1 tests "R" before the typed
    // ones, and every typed letter starts with 'R'.
    for (int i = 0; i <= 7; ++i) {
        const auto candidate = static_cast<SceneReferenceKind>(i);
        if (equals_no_case(letter, scene_reference_kind_letter(candidate))) {
            type = ScenePropertyType::Reference;
            kind = candidate;
            return true;
        }
    }
    if (equals_no_case(letter, "LUA_S")) {   // 00D162D8, the second type-4 letter
        type = ScenePropertyType::Enum;
        return true;
    }
    for (int i = 0; i <= 11; ++i) {
        if (i == static_cast<int>(ScenePropertyType::Reference) ||
            i == static_cast<int>(ScenePropertyType::SubBag)) {
            continue;
        }
        const auto candidate = static_cast<ScenePropertyType>(i);
        if (equals_no_case(letter, scene_property_type_letter(candidate))) {
            type = candidate;
            return true;
        }
    }
    return false;
}

int scene_property_value_token_count(ScenePropertyType type) noexcept {
    switch (type) {
        case ScenePropertyType::Int:
        case ScenePropertyType::Float:
        case ScenePropertyType::String:
        case ScenePropertyType::Bool:
        case ScenePropertyType::Reference:
            return 1;
        case ScenePropertyType::Enum:
            // 008F5E15..008F5E63: token, optional ":" (00CF00B0), token.
            return 3;
        case ScenePropertyType::Vector3:
            return 3;   // 008F62BF..008F62EB, three 008D9B40 reads
        case ScenePropertyType::SubBag:
            return 0;
        case ScenePropertyType::ByteArray:
        case ScenePropertyType::FloatArray:
        case ScenePropertyType::IntArray:
        case ScenePropertyType::Vector3Array:
            return kSceneVariableTokenCount;
    }
    return kSceneVariableTokenCount;
}

bool scene_property_type_is_counted(ScenePropertyType type) noexcept {
    return type == ScenePropertyType::ByteArray ||
           type == ScenePropertyType::FloatArray ||
           type == ScenePropertyType::IntArray ||
           type == ScenePropertyType::Vector3Array;
}

std::uint32_t scene_property_array_stride(ScenePropertyType type) noexcept {
    switch (type) {
        case ScenePropertyType::ByteArray: return 1;     // 008F6376, no multiply
        case ScenePropertyType::FloatArray: return 4;    // 008F6440 shift pair
        case ScenePropertyType::IntArray: return 4;      // 008F6540 [EBX+EBP*4]
        case ScenePropertyType::Vector3Array: return 12; // 008F65E7 MUL 0Ch
        default: return 0;
    }
}

std::uint32_t scene_property_array_bytes(ScenePropertyType type,
                                         std::uint32_t count) noexcept {
    const std::uint32_t stride = scene_property_array_stride(type);
    if (stride == 0) {
        return 0;
    }
    const std::uint64_t wide = static_cast<std::uint64_t>(count) * stride;
    if (wide > std::numeric_limits<std::uint32_t>::max()) {
        // SETO CL / NEG ECX / OR ECX,EAX: the request saturates, it does not wrap.
        return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(wide);
}

bool scene_decode_property_value(ScenePropertyType type,
                                 SceneReferenceKind kind,
                                 const std::vector<std::string>& tokens,
                                 ScenePropertyValue& out) noexcept {
    out = ScenePropertyValue{};
    out.type = type;
    switch (type) {
        case ScenePropertyType::Int: {
            if (tokens.size() != 1) {
                return false;
            }
            return parse_int(tokens[0], out.int_value);
        }
        case ScenePropertyType::Float: {
            if (tokens.size() != 1) {
                return false;
            }
            return parse_float(tokens[0], out.float_value);
        }
        case ScenePropertyType::String: {
            if (tokens.size() != 1) {
                return false;
            }
            out.text = tokens[0];
            return true;
        }
        case ScenePropertyType::Bool: {
            // 008F5D79/008F5DA1 test "true" (00CE4378) then "false" (00CE4370);
            // the stored byte is 1 only when "true" matched.
            if (tokens.size() != 1) {
                return false;
            }
            out.int_value = equals_no_case(tokens[0], "true") ? 1 : 0;
            return true;
        }
        case ScenePropertyType::Enum: {
            // `<decl> : <value>` keeps the value; `<value>` alone keeps that.
            // The second read overwrites the first buffer at ESP+0C0h, so the
            // declaration token is discarded once the ":" is consumed.
            if (tokens.size() == 1) {
                out.text = tokens[0];
                return true;
            }
            if (tokens.size() == 3 && tokens[1] == ":") {
                out.text = tokens[2];
                return true;
            }
            return false;
        }
        case ScenePropertyType::Reference: {
            if (tokens.size() != 1) {
                return false;
            }
            out.text = tokens[0];
            out.reference_kind = kind;
            return true;
        }
        case ScenePropertyType::SubBag:
            return false;   // a sub-block has no `=` value list
        case ScenePropertyType::Vector3: {
            if (tokens.size() != 3) {
                return false;
            }
            for (int i = 0; i < 3; ++i) {
                if (!parse_float(tokens[static_cast<std::size_t>(i)], out.vector[i])) {
                    return false;
                }
            }
            return true;
        }
        case ScenePropertyType::ByteArray:
        case ScenePropertyType::FloatArray:
        case ScenePropertyType::IntArray:
        case ScenePropertyType::Vector3Array: {
            // A bare ";" in the value position leaves the array empty: the
            // parser peeks for it (0045C440 at 008F635E/008F6424/008F64FC/
            // 008F65CA) before reading the count.
            if (tokens.empty() || tokens[0] == ";") {
                return true;
            }
            std::int32_t count = 0;
            if (!parse_int(tokens[0], count) || count < 0) {
                return false;
            }
            const std::size_t n = static_cast<std::size_t>(count);
            if (type == ScenePropertyType::IntArray) {
                if (tokens.size() != n + 1) {
                    return false;
                }
                out.int_array.resize(n);
                for (std::size_t i = 0; i < n; ++i) {
                    if (!parse_int(tokens[i + 1], out.int_array[i])) {
                        return false;
                    }
                }
                return true;
            }
            if (type == ScenePropertyType::FloatArray) {
                if (tokens.size() != n + 1) {
                    return false;
                }
                out.float_array.resize(n);
                for (std::size_t i = 0; i < n; ++i) {
                    if (!parse_float(tokens[i + 1], out.float_array[i])) {
                        return false;
                    }
                }
                return true;
            }
            if (type == ScenePropertyType::Vector3Array) {
                // `( x y z )` per element: 008F6610 expects "(" (00CE5850) and
                // 008F664C expects ")" (00CE584C) around three float reads.
                if (tokens.size() != 1 + n * 5) {
                    return false;
                }
                out.float_array.resize(n * 3);
                for (std::size_t i = 0; i < n; ++i) {
                    const std::size_t base = 1 + i * 5;
                    if (tokens[base] != "(" || tokens[base + 4] != ")") {
                        return false;
                    }
                    for (std::size_t c = 0; c < 3; ++c) {
                        if (!parse_float(tokens[base + 1 + c],
                                         out.float_array[i * 3 + c])) {
                            return false;
                        }
                    }
                }
                return true;
            }
            // ByteArray: one token per byte, converted by the host (008EF6D0),
            // so the rule only checks the shape.
            return tokens.size() == n + 1;
        }
    }
    return false;
}

const ScenePropertyEntry* scene_property_bag_find(const ScenePropertyBagModel& model,
                                                  int bag_index,
                                                  const std::string& key) noexcept {
    if (bag_index < 0 || static_cast<std::size_t>(bag_index) >= model.bags.size()) {
        return nullptr;
    }
    const std::size_t dot = key.find('.');
    const std::string head = (dot == std::string::npos) ? key : key.substr(0, dot);
    const ScenePropertyEntry* found = nullptr;
    for (const auto& entry : model.bags[static_cast<std::size_t>(bag_index)].entries) {
        if (equals_no_case(entry.key, head)) {
            found = &entry;
            break;
        }
    }
    if (dot == std::string::npos) {
        return found;
    }
    // 008F2318: the tail is followed only when the head record is type 6.
    if (found == nullptr || found->value.type != ScenePropertyType::SubBag) {
        return nullptr;
    }
    return scene_property_bag_find(model, found->sub_bag, key.substr(dot + 1));
}

ScenePropertyEntry& scene_property_bag_insert(ScenePropertyBagModel& model,
                                              int bag_index,
                                              const std::string& key,
                                              ScenePropertyValue value) noexcept {
    auto& bag = model.bags[static_cast<std::size_t>(bag_index)];
    for (auto& entry : bag.entries) {
        if (equals_no_case(entry.key, key)) {
            entry.value = std::move(value);
            return entry;
        }
    }
    ScenePropertyEntry entry;
    entry.key = key;
    entry.value = std::move(value);
    // 008F3411: the ordinal is taken only when the record has none. The counter
    // starts at 0 and 0 is also the "unstamped" sentinel, so the first record
    // inserted into a bag is re-stamped if it is ever inserted again.
    entry.ordinal = bag.next_ordinal;
    bag.next_ordinal += 1;
    bag.entries.push_back(std::move(entry));
    return bag.entries.back();
}

namespace {

int clone_bag_into(ScenePropertyBagModel& model,
                   const ScenePropertyBagModel& source_model,
                   int source_bag);

void clone_entry_into(ScenePropertyBagModel& model,
                      int dest_bag,
                      const ScenePropertyBagModel& source_model,
                      const ScenePropertyEntry& source) {
    ScenePropertyEntry entry;
    entry.key = source.key;
    entry.value = source.value;
    entry.ordinal = source.ordinal;   // 008F562E copies +34h onto the clone
    if (source.value.type == ScenePropertyType::SubBag) {
        entry.sub_bag = clone_bag_into(model, source_model, source.sub_bag);
    }
    auto& bag = model.bags[static_cast<std::size_t>(dest_bag)];
    bag.entries.push_back(std::move(entry));
    bag.next_ordinal = std::max(bag.next_ordinal, source.ordinal + 1);
}

int clone_bag_into(ScenePropertyBagModel& model,
                   const ScenePropertyBagModel& source_model,
                   int source_bag) {
    model.bags.emplace_back();
    const int index = static_cast<int>(model.bags.size()) - 1;
    if (source_bag < 0 ||
        static_cast<std::size_t>(source_bag) >= source_model.bags.size()) {
        return index;
    }
    // Copy the entry list by value so the loop is not invalidated when the
    // recursion appends to model.bags.
    const auto entries = source_model.bags[static_cast<std::size_t>(source_bag)].entries;
    for (const auto& entry : entries) {
        clone_entry_into(model, index, source_model, entry);
    }
    return index;
}

}  // namespace

int scene_property_bag_clone(ScenePropertyBagModel& model,
                             const ScenePropertyBagModel& source_model,
                             int source_bag) noexcept {
    return clone_bag_into(model, source_model, source_bag);
}

void scene_property_bag_merge(ScenePropertyBagModel& model,
                              int dest_bag,
                              const ScenePropertyBagModel& source_model,
                              int source_bag,
                              bool keep_existing) noexcept {
    if (dest_bag < 0 || static_cast<std::size_t>(dest_bag) >= model.bags.size()) {
        return;
    }
    if (source_bag < 0 ||
        static_cast<std::size_t>(source_bag) >= source_model.bags.size()) {
        return;
    }
    const auto entries = source_model.bags[static_cast<std::size_t>(source_bag)].entries;
    for (const auto& source : entries) {
        ScenePropertyEntry* target = nullptr;
        for (auto& entry : model.bags[static_cast<std::size_t>(dest_bag)].entries) {
            if (equals_no_case(entry.key, source.key)) {
                target = &entry;
                break;
            }
        }
        if (target == nullptr) {
            clone_entry_into(model, dest_bag, source_model, source);
            continue;
        }
        if (target->value.type == ScenePropertyType::SubBag) {
            // 008F55DE: the destination's type decides, and both sides' +0Ch
            // are the sub-bags handed to the recursion.
            scene_property_bag_merge(model, target->sub_bag, source_model,
                                     source.sub_bag, keep_existing);
            continue;
        }
        if (!keep_existing) {
            // 008F060D: 008F0700 assigns by the destination's type code.
            const ScenePropertyType destination_type = target->value.type;
            target->value = source.value;
            target->value.type = destination_type;
        }
    }
}

namespace {

// One typed assignment, after the `=` has been consumed and the letter settled.
bool read_typed_value(ScenePropertyBagHost& host,
                      ScenePropertyType type,
                      SceneReferenceKind kind,
                      bool allow_untyped_numbers,
                      ScenePropertyValue& out) {
    out = ScenePropertyValue{};
    out.type = type;
    bool ok = true;
    switch (type) {
        case ScenePropertyType::Int:
            // 008F5B92: with argument 2 clear, a non-numeric token yields 0 and
            // is not consumed.
            if (!allow_untyped_numbers && !host.next_token_is_number()) {
                out.int_value = 0;
                return true;
            }
            out.int_value = host.read_int(ok);
            return true;
        case ScenePropertyType::Float:
            if (!allow_untyped_numbers && !host.next_token_is_number()) {
                out.float_value = 0.0F;
                return true;
            }
            out.float_value = host.read_float(ok);
            return true;
        case ScenePropertyType::String:
            out.text = host.read_token();
            return true;
        case ScenePropertyType::Bool: {
            const bool is_true = host.peek_token() == "true";
            if (is_true || host.peek_token() == "false") {
                host.consume_token();
            }
            out.int_value = is_true ? 1 : 0;
            return true;
        }
        case ScenePropertyType::Enum: {
            out.text = host.read_token();
            if (host.peek_token() == ":") {
                host.consume_token();
                out.text = host.read_token();   // overwrites the first buffer
            }
            return true;
        }
        case ScenePropertyType::Reference:
            out.reference_kind = kind;
            if (host.peek_token().empty()) {
                return false;   // 008F60AE: an empty peek ends the property
            }
            out.text = host.read_token();
            return true;
        case ScenePropertyType::Vector3:
            for (int i = 0; i < 3; ++i) {
                if (!host.next_token_is_number()) {
                    return true;   // the remaining components stay 0.0
                }
                out.vector[i] = host.read_float(ok);
            }
            return true;
        case ScenePropertyType::ByteArray:
        case ScenePropertyType::FloatArray:
        case ScenePropertyType::IntArray:
        case ScenePropertyType::Vector3Array: {
            if (host.peek_token() == ";") {
                return true;   // empty array, no count read
            }
            const std::int32_t count = host.read_int(ok);
            if (!ok || count <= 0) {
                return true;
            }
            const std::size_t n = static_cast<std::size_t>(count);
            for (std::size_t i = 0; i < n; ++i) {
                if (type == ScenePropertyType::ByteArray) {
                    out.byte_array.push_back(host.decode_byte_token(host.read_token()));
                } else if (type == ScenePropertyType::IntArray) {
                    out.int_array.push_back(host.read_int(ok));
                } else if (type == ScenePropertyType::FloatArray) {
                    out.float_array.push_back(host.read_float(ok));
                } else {
                    host.expect_token("(");
                    for (int c = 0; c < 3; ++c) {
                        out.float_array.push_back(host.read_float(ok));
                    }
                    host.expect_token(")");
                }
            }
            return true;
        }
        case ScenePropertyType::SubBag:
            return false;
    }
    return false;
}

}  // namespace

ScenePropertyParseResult scene_property_bag_parse(ScenePropertyBagHost& host,
                                                  ScenePropertyBagModel& model,
                                                  int bag_index,
                                                  bool group_registry,
                                                  bool allow_untyped_numbers) {
    ScenePropertyParseResult result;
    result.bag_index = bag_index;
    if (bag_index < 0 || static_cast<std::size_t>(bag_index) >= model.bags.size()) {
        return result;
    }

    // 008F5A2E: the optional `( name name ... )` group list.
    if (host.peek_token() == "(" && group_registry) {
        host.expect_token("(");
        while (true) {
            const std::string name = host.peek_token();
            if (name.empty() || name == ")") {
                break;
            }
            const int group = host.lookup_group_bag(host.read_token());
            if (group >= 0) {
                // 008F5AB3 merges with keep_existing = 0: a group overwrites.
                scene_property_bag_merge(model, bag_index, model, group, false);
                result.groups_merged += 1;
            }
        }
        if (!host.expect_token(")")) {
            result.errors += 1;
        }
    }
    if (!host.expect_token("{")) {
        result.errors += 1;
    }

    while (true) {
        if (host.peek_token() == "}") {
            host.expect_token("}");
            result.ok = true;
            return result;
        }
        const std::string key = host.read_token();
        if (key.empty()) {
            return result;   // the tokenizer ran out before the closing brace
        }
        const ScenePropertyEntry* declared =
            scene_property_bag_find(model, bag_index, key);

        if (host.peek_token() != "=") {
            if (host.peek_token() == "{") {
                // 008F66D6: an existing type-6 record supplies the sub-bag,
                // otherwise a fresh 114h bag is allocated and inserted.
                int child = -1;
                if (declared != nullptr &&
                    declared->value.type == ScenePropertyType::SubBag) {
                    child = declared->sub_bag;
                } else {
                    model.bags.emplace_back();
                    child = static_cast<int>(model.bags.size()) - 1;
                    ScenePropertyValue value;
                    value.type = ScenePropertyType::SubBag;
                    auto& entry = scene_property_bag_insert(model, bag_index, key,
                                                            std::move(value));
                    entry.sub_bag = child;
                }
                scene_property_bag_parse(host, model, child, group_registry,
                                         allow_untyped_numbers);
                result.sub_blocks += 1;
            }
            // Neither `=` nor `{`: the key is dropped and the scan resumes.
            continue;
        }
        host.consume_token();

        // The letter is matched against the declared type first; an unmatched
        // letter falls through to the integer reader (008F5B50 -> 008F5B70).
        ScenePropertyType type = ScenePropertyType::Int;
        SceneReferenceKind kind = SceneReferenceKind::Any;
        const std::string letter = host.peek_token();
        ScenePropertyType letter_type = ScenePropertyType::Int;
        SceneReferenceKind letter_kind = SceneReferenceKind::Any;
        const bool letter_known =
            scene_property_type_for_letter(letter, letter_type, letter_kind);
        if (declared != nullptr) {
            type = declared->value.type;
            kind = declared->value.reference_kind;
            if (letter_known && letter_type == type) {
                host.consume_token();   // the letter only validates
                if (type == ScenePropertyType::Reference) {
                    kind = letter_kind;
                }
            }
        } else if (letter_known) {
            type = letter_type;
            kind = letter_kind;
            host.consume_token();
        }

        ScenePropertyValue value;
        if (read_typed_value(host, type, kind, allow_untyped_numbers, value)) {
            scene_property_bag_insert(model, bag_index, key, std::move(value));
            result.properties_read += 1;
        }
        // 008F669F: the terminator peeks and consumes only on a match, so a
        // property authored without `;` costs one error and consumes nothing.
        if (!host.expect_token(";")) {
            result.errors += 1;
        }
    }
}

}  // namespace bsp
