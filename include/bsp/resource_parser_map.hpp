#pragma once
#include <cstddef>
#include <map>
#include <string>

namespace bsp {
class StructuredResourceParser;

struct ResourceParserMapInsertResult {
    StructuredResourceParser* parser{};
    bool inserted{};
};

// Parser registry storage at native manager+8. Owns copied type-name bytes;
// parser values are borrowed and must outlive their use through this map.
// Native nodes are 28-byte red-black nodes; this host container has a new ABI
// and does not expose native iterators, allocation layout, or exception state.
// Evidence: docs/RESOURCE_PARSER_MAP_STORAGE.md and its companion byte audit.
class ResourceParserMap {
public:
    //00b80290 selects a unique node;00b7fd30 allocates/links/rebalances it.
    // Equivalent names retain the first parser and return that same identity.
    // Copies all counted key bytes, including bytes beyond an embedded NUL.
    // No AddRef, release, parser method, or key normalization is performed.
    ResourceParserMapInsertResult insert(const std::string& name,
        StructuredResourceParser* parser);
    //00b7e740: empty-length gates, then CRT _stricmp, no length tie-break.
    // A null return means absent or a stored null raw value; registration's
    // valid parser precondition belongs to its type-name virtual call.
    StructuredResourceParser* find(const std::string& name) const;
    // Host key-based convenience over find+00b7f790's native iterator erase.
    // Erasure frees the owned key/node and leaves the parser untouched.
    bool erase(const std::string& name);
    // Full-range00b80500/00b7f730 behavior: discard keys/nodes and reset size.
    void clear() noexcept;
    std::size_t size() const noexcept { return entries_.size(); }

private:
    struct NameLess {
        bool operator()(const std::string&, const std::string&) const noexcept;
    };
    std::map<std::string, StructuredResourceParser*, NameLess> entries_;
};
}
