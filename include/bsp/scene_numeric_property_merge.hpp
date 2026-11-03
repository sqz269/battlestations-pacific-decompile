#pragma once

#include "bsp/scene_file.hpp"

#include <cstddef>
#include <memory>

namespace bsp::game {

// Projects supported parser-owned I/F/B, decoded S, V3 and (chain 311) closed
// nonempty IA plus one-quoted-token reference values into two real native bags
// (the class name predates the S/V3/IA/R extensions; IA enters through
// 008F3590 as native type 10, the eight reference letters through 008F3AC0 as
// type 5 with their kind word).
// The whole native merge owns payload precedence; result() republishes those
// bytes with the selected authored spelling/tokens. Unsupported entries stay
// on the caller's existing host path. No Original class/stack ABI is supplied.

// Chain 319: the live library's native CEnum registry, the [00E1867C] object
// 004D3040 publishes and the parser's E arm reads through 0048E960. Returns the
// genuine CEnum for `table` only when the registry holds that table AND the
// table holds `symbol` (0048E960 and 0048E840 dereference a miss), else null.
class SceneNativeEnumSource {
public:
    virtual ~SceneNativeEnumSource() = default;
    virtual const void* known_enum_owner(const std::string& table, const std::string& symbol) = 0;
};

class SceneNumericPropertyMerge final {
public:
    // `enums` (chain 319) admits closed E values into the native merge through
    // 008F3A20; null leaves every E value on the host path.
    SceneNumericPropertyMerge(const ScenePropertyBlock& destination,
        const ScenePropertyBlock& source, bool overwrite,
        SceneNativeEnumSource* enums = nullptr);
    ~SceneNumericPropertyMerge();
    SceneNumericPropertyMerge(const SceneNumericPropertyMerge&) = delete;
    SceneNumericPropertyMerge& operator=(const SceneNumericPropertyMerge&) = delete;

    bool handles(std::size_t source_index) const;
    SceneProperty result(const SceneProperty& authored_metadata);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp::game
