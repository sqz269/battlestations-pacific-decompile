#pragma once
#include <cstdint>

namespace bsp {
// Projected native entry fields, not an ABI/layout overlay. Names are hypotheses.
struct RenderEntrySortFields {
    std::uint64_t key{};            // entry+20h (low DWORD), entry+24h (high DWORD)
    std::int32_t material_order{};   // entry+4 -> +20h -> +7Ch -> signed DWORD +B0h
    float secondary_value{};        // entry+14h; meaning not established
};

// Native ABI: ECX=left entry, EDX=right entry, RET; predicate consumed in AL.
// These new C++ interfaces do not dereference the original object graph.
bool render_entry_key_less_00b51b00(const RenderEntrySortFields& left,
                                 const RenderEntrySortFields& right) noexcept;
bool render_entry_material_value_less_00b51ab0(const RenderEntrySortFields& left,
                                            const RenderEntrySortFields& right) noexcept;
// NaN secondary values compare false in both directions within a material group.
// Such inputs do not form a strict weak ordering: do not pass that domain to
// std::sort. The native sort algorithm and equal-item permutation are unported.
}
