#include "bsp/structured_resource_registry.hpp"
#include "bsp/structured_resource.hpp"
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
MeshStructuredResourceParser::MeshStructuredResourceParser(
    MeshVertexFormatResolver resolve_format)
    : resolve_format_(std::move(resolve_format)) {}

std::string MeshStructuredResourceParser::type_name() const { return "Mesh"; }
std::string NoteStructuredResourceParser::type_name() const { return "Note"; }
std::string GroupParamsStructuredResourceParser::type_name() const { return "GroupParams"; }

bool MeshStructuredResourceParser::decode(StructuredNode& node,
    StructuredResourcePayload& output, std::string& error) {
    //00b947a0 wraps the mesh/prefix pair decoded through00b944e0. Keep that
    // actual payload (including prefix_word) without constructing its item.
    MeshResourcePayload parsed;
    if (!parse_mesh_resource_00b944e0(node, resolve_format_, parsed, error)) return false;
    output = std::move(parsed);
    return true;
}

bool NoteStructuredResourceParser::decode(StructuredNode& node,
    StructuredResourcePayload& output, std::string& error) {
    //00719000 ->00718f50: full counted read, then retain the C-string prefix.
    error.clear();
    NoteResourcePayload parsed;
    if (!read_note_text_00718f50_fragment(node, parsed.text)) {
        error = "Could not read Note text.";
        return false;
    }
    output = std::move(parsed);
    return true;
}

bool GroupParamsStructuredResourceParser::decode(StructuredNode& node,
    StructuredResourcePayload& output, std::string& error) {
    //00b8eb50 ->00b8e580: read float and explicitly skip/detach the remainder.
    error.clear();
    GroupParamsResourcePayload parsed;
    if (!read_group_params_00b8e580_fragment(node, parsed.value)) {
        error = "Could not read GroupParams value.";
        return false;
    }
    output = parsed;
    return true;
}

bool StructuredResourceRegistry::register_parser(StructuredResourceParser& parser) {
    {
        const auto probe_name = parser.type_name();
        if (parsers_.find(probe_name)) return false;
    }
    const auto insertion_name = parser.type_name();
    parsers_.insert(insertion_name, &parser);
    // Native registration does not inspect the unique-insertion result.
    return true;
}

StructuredResourceParser* StructuredResourceRegistry::find_parser(
    const std::string& name) const {
    return parsers_.find(name);
}

void StructuredResourceRegistry::clear() noexcept { parsers_.clear(); }

bool StructuredResourceRegistry::dispatch_items_00b7e970(StructuredNode& container,
    std::vector<DecodedStructuredResource>& output, std::string& error) {
    error.clear();
    if (!container.ready()) {
        error = "Resource container is not ready.";
        return false;
    }
    try {
        while (container.has_remaining_00715bf0()) {
            auto child = container.read_child_00bea680();
            if (!child) {
                error = "Could not read Resource child header.";
                return false;
            }
            DecodedStructuredResource item;
            item.type_name = child->tag();
            if (auto* parser = parsers_.find(item.type_name)) {
                StructuredResourcePayload decoded;
                if (!parser->decode(*child, decoded, error)) return false;
                item.payload = std::move(decoded);
            } else if (!child->skip_00be9c40()) {
                error = "Could not skip unsupported Resource payload.";
                return false;
            }
            // Native appends the item before releasing its child node handle.
            output.push_back(std::move(item));
            if (!child->close()) {
                error = "Could not close Resource child.";
                return false;
            }
        }
        if (!container.ready()) {
            error = "Resource reader failed before completion.";
            return false;
        }
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate decoded Resource records.";
        return false;
    } catch (const std::length_error&) {
        error = "Decoded Resource records exceed host container capacity.";
        return false;
    }
}
}
