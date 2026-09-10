#pragma once
#include "bsp/structured_reader.hpp"
#include <array>
#include <string>
#include <variant>
#include <vector>

namespace bsp {
// Serialized fields are owning host values, not native mesh/material layouts.
// No effect or texture object is realized by these parsers. Names remain exact
// counted strings (including embedded NULs), except the native effect alias.
// Evidence: docs/MESH_SUBSET_LOD_FIELDS.md.
struct MeshLodPhase {
    float value0{};
    float value1{};
    std::uint32_t word0{};
    std::uint32_t word1{};
};

struct MeshTextureRequest {
    std::string name;
    std::uint32_t slot{};
};

struct MeshLightingRecord {
    // Native00b179d0 ignores this serialized argument and overwrites the same
    // material+38h record each time. The final lighting event wins.
    std::uint32_t ignored_slot{};
    std::array<float, 17> values{};
};

struct MeshVertexStreamReference {
    std::uint32_t index{};
};

using MeshSubsetEvent = std::variant<MeshTextureRequest, MeshLightingRecord,
    MeshVertexStreamReference>;

struct MeshSubsetFields {
    std::uint32_t serialized_primitive{};
    std::uint32_t native_primitive{};
    // Exact words written to native section+0Ch,+10h,+14h,+18h. Their draw-call
    // interpretation is deliberately left to the audited geometry consumer.
    std::array<std::uint32_t, 4> range_words{};
    std::string effect_name;
    std::vector<MeshSubsetEvent> events;
};

// Reader helpers leave their input node attached. Recognized child handles
// close without seeking any unread tail; unknown children explicitly skip.
// Host failures consume input but preserve output (no native rollback claim).
// A resource decoder must resolve requests before claiming realized materials.
bool parse_mesh_subset_00b941d0(StructuredNode& node,
    MeshSubsetFields& output, std::string& error);

// No synthetic stream-zero event: native subset finalization uses stream zero
// only when the section's selected-stream count is zero. Runtime binding must
// resolve the selected index against mesh streams in serialized field order.

// Native00b93710 appends via00b73270 without checking its four inline slots.
// This host complete-transfer domain rejects totals above four before reading
// records, preventing overwrite of native count+50h and subsequent fields.
bool append_mesh_lod_phases_00b93710(StructuredNode& node,
    std::vector<MeshLodPhase>& output, std::string& error);

// The node read is the00b944e0 LODValue dispatch fragment. Its native setter
//00b72710 receives a stack float and stores its bits at ECX(mesh)+0Ch.
bool read_mesh_lod_value_00b944e0_fragment(StructuredNode& node,
    float& output) noexcept;

// Native mesh-specific wrappers discard every float with FSTP ST0; these are
// not the hierarchy sphere/box setters and do not populate mesh bounds.
bool consume_mesh_sphere_00b93590(StructuredNode& node) noexcept;
bool consume_mesh_box_00b935c0(StructuredNode& node) noexcept;

// No leading count: append independent string copies until payload remaining
// is zero, as00b93f90 ->00b73d50 ->004cdc20 does for mesh+B0h.
bool append_mesh_weight_map_names_00b93f90(StructuredNode& node,
    std::vector<std::string>& output, std::string& error);
}
