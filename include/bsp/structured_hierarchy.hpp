#pragma once
#include "bsp/structured_reader.hpp"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace bsp {
// Owning host values for00b7eb90's132-byte native record, not its layout/ABI.
// Parent/resources remain raw serialized DWORDs: no index lookup or graph
// validation. Name preserves its stored byte length, including embedded NULs.
struct HierarchyItem {
    std::uint32_t parent{0xffffffffu};
    std::string name;
    std::vector<std::uint32_t> resources;
    // Native allocation/caller leave absent Matrix storage uninitialized.
    // Explicit absence is a host representation; never substitute identity.
    std::optional<std::array<float, 16>> matrix;
    std::uint32_t flags{};
    std::array<float, 4> sphere{0.0f, 0.0f, 0.0f, 1.0e10f};
    std::array<float, 6> box{
        -1.0e10f, -1.0e10f, -1.0e10f, 1.0e10f, 1.0e10f, 1.0e10f};
};

// Original ECX node wrapper, EDX destination; no stack arguments, plain RET.
// Exactly16/four sequential float32 stores. Host failure may leave a prefix
// written; short-read native pointer-seed behavior is outside the typed domain.
bool read_matrix_00b936e0(StructuredNode& node,
    std::array<float, 16>& matrix) noexcept;
bool read_sphere_00b932e0(StructuredNode& node,
    std::array<float, 4>& sphere) noexcept;

// Rounds center-plus-radius XYZ before center-minus-radius XYZ through x87
// float32 temporaries. Output is minima XYZ then maxima XYZ. No validation.
// Native wrapper00b7d220 copies this result through a temporary output.
// Exceptional x87 control/status and NaN behavior remain unverified.
void sphere_to_box_00b7d160(const std::array<float, 4>& sphere,
    std::array<float, 6>& box) noexcept;

// Original00b7eb90: ECX manager, stack node-wrapper pointer, RET4; final record
// append/ownership is separate. This host parser does not close its input node.
// Field tags use CRT case-insensitive C-string comparisons. Unknown fields are
// explicitly skipped; recognized fields close WITHOUT an implicit payload seek.
// Parsing advances the stream even on failure; output is replaced only after
// complete success (host error policy, not recovered native rollback).
// Evidence: docs/STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md and
// docs/STRUCTURED_HIERARCHY_VALUES.md. No native ABI or game-validation claim.
bool parse_hierarchy_item_00b7eb90(StructuredNode& item,
    HierarchyItem& output, std::string& error);
}
