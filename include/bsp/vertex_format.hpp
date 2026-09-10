#pragma once
#include "bsp/mesh_buffers.hpp"
#include "bsp/vertex_declaration.hpp"
#include <string>
#include <string_view>

namespace bsp {
// Native00b2dbd0 expands 54 aliases and decodes usage/type tokens, requiring a
// .mvfm suffix and at least one element. It does not load a file. This owning
// host API excludes embedded NUL and allocation/length overflow cases; it
// preserves output on failure. Native registry ownership/ABI are separate.
// Evidence: docs/VERTEX_FORMAT_RESOLUTION.md.
bool decode_vertex_format_00b2dbd0(std::string_view name,
    VertexDeclaration& output, std::string& error);

// Supplies real declaration-derived sizes to the structured mesh reader.
bool resolve_mesh_vertex_format_layout_00b2dbd0(const std::string& name,
    MeshVertexFormatLayout& output, std::string& error);
}
