#pragma once
#include "bsp/structured_reader.hpp"
#include <array>

namespace bsp {
// Six scalar fields: minimum XYZ, then maximum XYZ. No sorting or validation
// of the float values. New host interface; original ECX reader-handle pointer,
// EDX output pointer, no stack arguments. Evidence:00b93310 and
// docs/STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md.
bool read_bounding_box_00b93310(StructuredNode& node,
    std::array<float, 6>& bounds) noexcept;
// Payload of the Note item parser. Native00718f50 converts its temporary
// counted string through strlen before assignment, discarding bytes after NUL.
// Does not construct the native0x28-byte reference-counted resource item.
bool read_note_text_00718f50_fragment(StructuredNode& node,
    std::string& text) noexcept;
// GroupParams item virtual parser: ECX item, stack node handle, RET4. Reads
// one float32 into native item+8, then explicitly skips/detaches the remaining
// node payload. No resource object or semantic label for the float is inferred.
// Host output commits after a successful skip; input is not rolled back.
// Evidence: docs/MESH_SUBSET_LOD_FIELDS.md (GroupParams dependency audit).
bool read_group_params_00b8e580_fragment(StructuredNode& node,
    float& output) noexcept;
}
