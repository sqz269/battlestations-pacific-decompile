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
// The 20h bytes attached by00b93800 and addressed by00b61e10 contain two
// little-endian float4s. Values remain in the shader's D3D declaration domain;
// reading this record does not decompress or rewrite any vertex bytes.
struct MeshVertexDecodeRecord {
    std::array<float, 4> scale{{1.0f, 1.0f, 1.0f, 1.0f}};
    std::array<float, 4> offset{};
};

// Host checked value projection of native ECX stream, stack element index,
// EAX=[stream+50]+index*20h, RET4. The native getter has no checks. This
// interface rejects absent/short metadata and leaves output unchanged.
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
// Host source/destination bounds validation is atomic: errors leave words and
// stats unchanged. Overlapping destinations retain native element write order.
// x87 NaN payload conversion/FP exception state is outside this value projection.
// Evidence: docs/MESH_VERTEX_DECODE_BINDING.md.
bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const MeshVertexStreamPayload*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error);
}
