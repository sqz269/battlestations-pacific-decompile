#pragma once
#include "bsp/d3d9_states.hpp"
#include <array>
#include <cstdint>
#include <memory>

namespace bsp {
inline constexpr DWORD generated_instance_stream_flags = 0x1000;
inline constexpr DWORD generated_instance_stream_tag = 0x80000000;
inline constexpr UINT generated_instance_shared_vertex_capacity = 0x1000000;

// Value/ownership projection of the one generated draw section. Native +8h
// is primitive, +Ch/+10h/+14h/+18h are the copied range words, +1Ch is the
// initially zero instance count, and +34h is initially zero depth bias.
struct GeneratedInstanceSection {
    std::uint32_t primitive{};
    std::array<std::uint32_t, 4> range_words{};
    std::uint32_t instance_count{};
    float depth_bias{};
    // Values read by the upload/queue consumer from the cloned material's
    // retained effect: [[section+20h]+7Ch]+B0h/+ACh, respectively.
    std::int32_t material_order{};
    std::uint32_t material_queue_index{};
    std::shared_ptr<void> material_clone_owner;
};

struct GeneratedInstanceGeometry {
    // Native mesh and section each retain these streams. One shared host owner
    // per field preserves reachability without reproducing intrusive counts.
    std::shared_ptr<LogicalVertexStream> mesh_stream;
    std::shared_ptr<LogicalVertexStream> instance_stream;
    std::shared_ptr<LogicalIndexStream> indices;
    std::shared_ptr<D3D9VertexLayout> combined_layout;
    GeneratedInstanceSection section;
};

struct GeneratedInstanceGeometrySource {
    // Selected source section+3Ch, not necessarily source mesh stream zero.
    std::shared_ptr<LogicalVertexStream> selected_mesh_stream;
    std::shared_ptr<LogicalIndexStream> indices; // Source mesh+60h; may be null.
    std::uint32_t primitive{};
    std::array<std::uint32_t, 4> range_words{};
};

struct GeneratedInstanceGeometryGenerator {
    std::shared_ptr<VertexDeclaration> instance_declaration; // Generator+10h.
    // Generator+14h, already combined in mesh,instance order by00b55b20.
    // The allocator retains this identity; it does not rebuild the layout.
    std::shared_ptr<D3D9VertexLayout> combined_layout;
};

// Explicit unresolved native-owner boundary. Native00b18b60 allocates a
// distinct material and retains/copies its selected fields. The caller must
// provide that clone and matching effect queue values. A host diagnostic
// material can exercise the buffer fragment but is not a native clone claim.
struct GeneratedInstanceMaterialClone {
    std::shared_ptr<void> owner;
    std::int32_t material_order{};
    std::uint32_t material_queue_index{};
};

// Native00b4c8d0: ECX model constructor argument, EDX generator, stack source
// mesh then selected section; EAX generated model wrapper; RET8. This typed
// fragment reconstructs the retained geometry/stream/section success path.
// Wrapper00b75030/00b75170, material cloning, CPU pools and renderer raw-object
// registries are separate contracts. It returns no native model pointer.
//
// renderer_shared_vertices is the existing renderer+1974h wrapper initialized
// by00b2aeb0 (flags1000h,16MiB,DEFAULT,DYNAMIC|WRITEONLY). No buffer is created,
// resized, cleared, locked or rewound here. Declaration/layout must correspond
// to the source/generator pair, and the physical wrapper must remain stable.
// states must outlive every resulting instance-stream reference, including
// references retained by its binding cache. Unbind/invalidate before teardown.
// Registration remains active until the final stream reference is released.
// Output changes only on success; checked HRESULTs/rollback are host policy.
HRESULT create_generated_instance_geometry_00b4c8d0_fragment(
    D3D9StateCache& states,
    const std::shared_ptr<VertexBufferBinding>& renderer_shared_vertices,
    const GeneratedInstanceGeometrySource& source,
    const GeneratedInstanceGeometryGenerator& generator,
    const GeneratedInstanceMaterialClone& material_clone,
    std::shared_ptr<GeneratedInstanceGeometry>& output);
}
