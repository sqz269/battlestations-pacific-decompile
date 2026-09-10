#include "bsp/structured_resource.hpp"
#include <utility>

namespace bsp {
bool read_bounding_box_00b93310(StructuredNode& node,
    std::array<float, 6>& bounds) noexcept {
    // Preserve ordered writes. Host failure can leave earlier fields written;
    // native short-read behavior is outside the reader's complete-read domain.
    for (float& value : bounds) if (!node.read_float(value)) return false;
    return true;
}

bool read_note_text_00718f50_fragment(StructuredNode& node,
    std::string& text) noexcept {
    std::string temporary;
    if (!node.read_string(temporary)) return false;
    const auto nul = temporary.find('\0');
    if (nul != std::string::npos) temporary.resize(nul);
    text = std::move(temporary);
    return true;
}
}
