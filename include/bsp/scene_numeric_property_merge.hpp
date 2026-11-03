#pragma once

#include "bsp/scene_file.hpp"

#include <cstddef>
#include <memory>

namespace bsp::game {

// Projects supported parser-owned I/F/B, decoded S, V3 and (chain 311) closed
// nonempty IA plus one-quoted-token reference values, and (chain 321) closed
// nonempty BIN byte payloads into two real native bags
// (the class name predates the S/V3/IA/R extensions; IA enters through
// 008F3590 as native type 10, the eight reference letters through 008F3AC0 as
// type 5 with their kind word).
// The whole native merge owns payload precedence; result() republishes those
// bytes with the selected authored spelling/tokens. Unsupported entries stay
// on the caller's existing host path. No Original class/stack ABI is supplied.
class SceneNumericPropertyMerge final {
public:
    SceneNumericPropertyMerge(const ScenePropertyBlock& destination,
        const ScenePropertyBlock& source, bool overwrite);
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
