#pragma once
#include "bsp/mesh_resource.hpp"
#include "bsp/resource_parser_map.hpp"
#include <optional>
#include <variant>
#include <vector>

namespace bsp {
struct NoteResourcePayload {
    std::string text;
};
struct GroupParamsResourcePayload {
    float value{};
};
using StructuredResourcePayload = std::variant<MeshResourcePayload,
    NoteResourcePayload, GroupParamsResourcePayload>;

struct DecodedStructuredResource {
    // The original counted child tag, including bytes after an embedded NUL.
    std::string type_name;
    // nullopt explicitly records an unsupported type whose payload was skipped.
    // Native00b7e970 allocates an eight-byte fallback item in this branch;
    // this host record does not construct or stand in for that native object.
    std::optional<StructuredResourcePayload> payload;
};

// Host interface for native parser virtual+4 (owned type-name result) and
// virtual+8 (node-handle parse). The payload is owning decoded wire data,
// not the native resource item's reference-counted object or binary ABI.
class StructuredResourceParser {
public:
    virtual ~StructuredResourceParser() = default;
    virtual std::string type_name() const = 0;
    virtual bool decode(StructuredNode& node, StructuredResourcePayload& output,
        std::string& error) = 0;
};

class MeshStructuredResourceParser final : public StructuredResourceParser {
public:
    explicit MeshStructuredResourceParser(MeshVertexFormatResolver resolve_format);
    std::string type_name() const override;
    bool decode(StructuredNode&, StructuredResourcePayload&, std::string&) override;
private:
    MeshVertexFormatResolver resolve_format_;
};

class NoteStructuredResourceParser final : public StructuredResourceParser {
public:
    std::string type_name() const override;
    bool decode(StructuredNode&, StructuredResourcePayload&, std::string&) override;
};

class GroupParamsStructuredResourceParser final : public StructuredResourceParser {
public:
    std::string type_name() const override;
    bool decode(StructuredNode&, StructuredResourcePayload&, std::string&) override;
};

// Borrowed parsers must remain alive while registered. Callers explicitly
// register actual implementations; this does not synthesize the native six
// constructor singletons or extend their separately registered lifetimes.
// Evidence: docs/RESOURCE_PARSER_MAP_STORAGE.md and registration/dispatch audits.
class StructuredResourceRegistry {
public:
    //00b80a50: obtain name once for the probe, again only after a miss. Returns
    // false on the first-name duplicate; after a miss returns true even if a
    // changed second name collides and unique insertion retains another parser.
    bool register_parser(StructuredResourceParser& parser);
    StructuredResourceParser* find_parser(const std::string& name) const;
    void clear() noexcept;
    std::size_t size() const noexcept { return parsers_.size(); }

    //00b7e970's ordered Resource-child dispatch. Appends successful decoded or
    // skipped records to output, then closes each child without an implicit
    // unread-tail seek. GroupParams and unknown handlers explicitly skip.
    // Leaves the Resource container attached. On failure, consumed input and
    // already appended records remain; no stream/output rollback is promised.
    // Renderer hooks, native item creation/append/lifetime remain separate.
    bool dispatch_items_00b7e970(StructuredNode& container,
        std::vector<DecodedStructuredResource>& output, std::string& error);
private:
    ResourceParserMap parsers_;
};
}
