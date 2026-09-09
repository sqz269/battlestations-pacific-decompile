#include "bsp/vertex_declaration.hpp"
#include <cstdlib>

namespace bsp {
namespace {
// Exact DWORD table at 00d61cc0. Do not derive stride from maximum offset.
constexpr std::uint32_t sizes[] = {4,8,12,16,4,4,4,8,4,4,8,4,8,4,4,4,8};
std::uint32_t type_size(std::uint32_t type) {
    if (type >= 17) std::abort(); // New API bounds precondition.
    return sizes[type];
}
}
void VertexDeclaration::append_00b48330(std::uint32_t type, std::uint32_t usage, std::int32_t offset) {
    if (usage >= by_usage_.size()) std::abort();
    const std::uint32_t bytes = type_size(type);
    VertexElement value{offset == -1 ? packed_offset_ : static_cast<std::uint32_t>(offset),
        type, 0, usage, -1};
    packed_offset_ += bytes; // Explicit offsets do not change the cursor rule.
    elements_.push_back(value);
    by_usage_[usage].push_back(value);
    recompute_stride_00b47d20();
}
void VertexDeclaration::recompute_stride_00b47d20() {
    stride = 0;
    for (const auto& value : elements_) stride += type_size(value.type);
}
std::int32_t VertexDeclaration::find_00b47ce0(std::uint32_t usage, std::int32_t occurrence) const {
    std::int32_t seen = 0;
    for (std::size_t i = 0; i < elements_.size(); ++i) {
        if (elements_[i].usage == usage && seen++ == occurrence) return static_cast<std::int32_t>(i);
    }
    return -1;
}
bool VertexDeclaration::contains_00b47c90(std::uint32_t usage, std::int32_t occurrence) const {
    // Native truth value is AL; upper EAX bits are incidental iteration state.
    return find_00b47ce0(usage, occurrence) != -1;
}
const VertexElement& VertexDeclaration::element(std::uint32_t usage, std::uint32_t occurrence) const {
    if (usage >= by_usage_.size() || occurrence >= by_usage_[usage].size()) std::abort();
    return by_usage_[usage][occurrence];
}
std::uint32_t VertexDeclaration::offset_00b47c40(std::uint32_t usage, std::uint32_t occurrence) const {
    return element(usage, occurrence).offset;
}
std::uint32_t VertexDeclaration::type_00b47c20(std::uint32_t usage, std::uint32_t occurrence) const {
    return element(usage, occurrence).type;
}
std::uint32_t VertexDeclaration::size_00b47c60(std::uint32_t usage, std::uint32_t occurrence) const {
    return type_size(type_00b47c20(usage, occurrence));
}
}
