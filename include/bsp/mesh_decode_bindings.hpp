#pragma once
#include "bsp/mesh_buffers.hpp"
#include "bsp/shader_reflection.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bsp {
struct LogicalVertexStream;
// The 20h bytes attached by00b93800 and addressed by00b61e10 contain two
// little-endian float4s. Values remain in the shader's D3D declaration domain;
// reading this record does not decompress or rewrite any vertex bytes.
struct MeshVertexDecodeRecord {
    std::array<float, 4> scale{{1.0f, 1.0f, 1.0f, 1.0f}};
    std::array<float, 4> offset{};
};

// Host checked value projection of native ECX stream, stack element index,
// EAX=[stream+50]+index*20h, RET4. The native getter has no checks. This
// interface rejects absent/short metadata and leaves output unchanged. It
// returns source bits without FP conversion; packing below applies native x87.
bool read_mesh_vertex_decode_record_00b61e10(const MeshVertexStreamPayload&,
    std::uint32_t element_index, MeshVertexDecodeRecord& output,
    std::string& error);

// Fields at descriptor[[pass+14]+C4]+20/+24. +20 is the primary descriptor's
// CompressedElemCount;00b465c8..00b465dd copies the ShadowShader descriptor's
// +20 into primary+24. Selector2 requires that explicit shadow value; absence
// is a host error rather than an invented fallback to the primary limit.
struct MeshDecodeDescriptorLimits {
    std::uint32_t element_limit20{};
    std::optional<std::uint32_t> element_limit24;
};

struct MeshDecodeBindingStats {
    bool scale_register_absent{};
    std::size_t streams_visited{};
    std::size_t records_written{};
    std::size_t records_from_metadata{};
    std::uint32_t remaining_gap{};
    std::uint32_t remaining_descriptor_limit{};
};

// Material-pass fragment00b428c0..00b42a7b, not a standalone original ABI.
// Parent00b42350: ECX pass, stack entry/override, RET8. ordered_streams is the
// selected draw section's +3C stream pointer order (+4C count), allowing repeated
// or reordered parsed streams. Indexing restarts at zero for each stream.
// Reflection semantic24/25 supplies scale/offset registers; only scale==FF
// skips. counts[] does not clamp writes. Each stream uses signed
// min(declaration element count, offset cursor - scale cursor). The separate
// DWORD remaining gap and selected descriptor limit wrap on subtraction and
// are tested for zero only at stream boundaries, exactly as in the assembly.
//
// Writes only visited float4 pairs into the caller's existing VS word vector;
// never clears/resizes it. Null metadata supplies (1,1,1,1)/(+0,+0,+0,+0).
// Each selected record writes immediately, in scale0123 then offset0123 order,
// using eight x87 FLD m32/FSTP m32 pairs. No whole-record or whole-call staging;
// signaling NaNs quiet/set invalid as native, under the caller's x87 control.
// Host errors retain completed writes; stats reset at entry and track completed
// records/visited streams even on failure. Per-record source/destination bounds
// checks are new host safeguards, not native error paths. Overlapping output
// registers retain native element order. No callbacks run within this concrete
// owner loop; containers/owners must remain valid for its duration.
// Evidence: docs/MESH_VERTEX_DECODE_BINDING.md.
bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const MeshVertexStreamPayload*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error);

// Same fragment over actual live draw-section owners. Resolve these streams
// after preceding builder callbacks. Concrete native virtual+24 is00B48CE0,
// a plain stream+68 declaration getter;00B47900 reads declaration+10 count.
// This overload reads that retained declaration's elements() and the SAME
// stream.compressed_format_bytes_50 backing. Presence is captured per stream
// before the declaration read; record backing is fetched afresh per element.
// Generated instance streams use their real constructor-null metadata state.
// No separate parsed-payload snapshot or synthetic declaration is constructed.
bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const LogicalVertexStream*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error);
}
