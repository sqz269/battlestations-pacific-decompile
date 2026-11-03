#pragma once

#include "bsp/scene_file.hpp"

#include <cstddef>
#include <memory>

namespace bsp::game {

// Projects supported parser-owned I/F/B values into two real native bags.
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
