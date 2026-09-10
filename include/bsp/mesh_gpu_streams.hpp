#pragma once
#include "bsp/d3d9_states.hpp"
#include "bsp/mesh_buffers.hpp"
#include <memory>

namespace bsp {
// Installed mesh handlers supply flags 1: dedicated MANAGED buffers, usage 0.
// These success-path projections preserve the serialized GPU bytes and retain
// stable logical/physical owners. They do not reproduce native renderer raw
// registries, CPU object slabs, diagnostics or the device-recreation retry.
// device must be the device used by states. The output changes only on success.
// Declaration comes from the actual named format, never from payload size.
// Native ctor ABI: ECX stream; stack count,declaration,flags; EAX this; RET Ch.
HRESULT create_mesh_vertex_stream_00b4bc00_fragment(IDirect3DDevice9& device,
    D3D9StateCache& states, const MeshVertexStreamPayload& payload,
    const std::shared_ptr<VertexDeclaration>& declaration,
    std::shared_ptr<LogicalVertexStream>& output);

// Native ctor ABI: ECX stream; stack count,format,flags; EAX this; RET Ch.
// INDEX16/INDEX32 is taken directly from wire format 65h/66h. This deliberately
// does not use shared-buffer recreation, whose native format is always INDEX16.
HRESULT create_mesh_index_stream_00b4bf30_fragment(IDirect3DDevice9& device,
    D3D9StateCache& states, const MeshIndexPayload& payload,
    std::shared_ptr<LogicalIndexStream>& output);
}
