#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace bsp::game {

// A stable link to 0077E864's owning creation row, not a flight-leader link.
// Unit slots are append-only for the mission; member promotion/removal and
// shifting logical squadron candidate IDs must not reseat this owner.
struct AiSquadronResourceOwner {
    std::size_t creation_unit{std::numeric_limits<std::size_t>::max()};
};

template<class UnitRowAt>
float ai_candidate_resource_usage(std::size_t candidate, std::size_t unit_count,
    const AiSquadronResourceOwner* squadron, UnitRowAt unit_row) {
    std::size_t owner = candidate;
    if (candidate >= unit_count) {
        if (squadron == nullptr) throw std::logic_error("AI resource candidate has no squadron");
        owner = squadron->creation_unit;
    }
    const auto* row = unit_row(owner);
    if (row == nullptr) throw std::logic_error("AI resource owner has no unit row");
    return row->resource_usage_304;
}

} // namespace bsp::game
