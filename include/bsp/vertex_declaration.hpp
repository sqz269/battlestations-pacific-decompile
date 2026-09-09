#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace bsp {
// Native declaration records are five DWORDs, not D3DVERTEXELEMENT9.
struct VertexElement {
    std::uint32_t offset{};
    std::uint32_t type{};
    std::uint32_t method{};
    std::uint32_t usage{};
    std::int32_t usage_index{-1}; // Exact purpose/assignment of this sentinel pending.
};
static_assert(sizeof(VertexElement) == 20);

class VertexDeclaration {
public:
    void append_00b48330(std::uint32_t type, std::uint32_t usage, std::int32_t offset = -1);
    void recompute_stride_00b47d20();
    bool contains_00b47c90(std::uint32_t usage, std::int32_t occurrence) const;
    std::int32_t find_00b47ce0(std::uint32_t usage, std::int32_t occurrence) const;
    std::uint32_t offset_00b47c40(std::uint32_t usage, std::uint32_t occurrence) const;
    std::uint32_t type_00b47c20(std::uint32_t usage, std::uint32_t occurrence) const;
    std::uint32_t size_00b47c60(std::uint32_t usage, std::uint32_t occurrence) const;
    const std::vector<VertexElement>& elements() const { return elements_; }
    std::uint32_t stride{}; // Native +cch, consumed by stream binding.
private:
    const VertexElement& element(std::uint32_t usage, std::uint32_t occurrence) const;
    std::uint32_t packed_offset_{}; // Native +8h.
    std::vector<VertexElement> elements_; // Native array at +ch.
    std::array<std::vector<VertexElement>, 15> by_usage_; // Native arrays at +18h.
};
}
