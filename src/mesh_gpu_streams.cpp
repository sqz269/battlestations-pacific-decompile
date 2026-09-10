#include "bsp/mesh_gpu_streams.hpp"
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace bsp {
namespace {
bool payload_capacity(std::uint32_t count, std::uint32_t width,
    std::size_t size, UINT& capacity) {
    const std::uint64_t bytes = std::uint64_t(count) * width;
    if (!count || !width || bytes > (std::numeric_limits<UINT>::max)()
        || bytes != size) return false;
    capacity = static_cast<UINT>(bytes);
    return true;
}
}

HRESULT create_mesh_vertex_stream_00b4bc00_fragment(IDirect3DDevice9& device,
    D3D9StateCache& states, const MeshVertexStreamPayload& payload,
    const std::shared_ptr<VertexDeclaration>& declaration,
    std::shared_ptr<LogicalVertexStream>& output) {
    UINT capacity = 0;
    if (!declaration || declaration->stride != payload.layout.stride
        || declaration->elements().size() != payload.layout.element_count
        || (payload.has_compressed_data
            && std::uint64_t(payload.layout.element_count) * 0x20
                != payload.compressed_format_bytes.size())
        || !payload_capacity(payload.count, declaration->stride,
            payload.bytes.size(), capacity)) return E_INVALIDARG;
    try {
        auto candidate = std::make_shared<LogicalVertexStream>();
        candidate->physical = std::make_shared<VertexBufferBinding>();
        candidate->declaration = declaration;
        // The complete parsed mesh already includes00B93800's later+50
        // attachment. Retain those exact bytes on this actual logical owner;
        // this is separate from00B4BC00's native constructor-null default.
        if (payload.has_compressed_data)
            candidate->compressed_format_bytes_50 = payload.compressed_format_bytes;
        candidate->vertex_count = payload.count;
        candidate->flags = 1;
        candidate->tag = 0x40000000; // Base constructor 00b61ea6.
        candidate->physical->flags = 1;
        candidate->physical->capacity = capacity;
        // 00b4bd8e..00b4be84: private buffer, original flags/capacity,
        // retained COM object and offset 0. Shared registry is dynamic-only.
        HRESULT result = vertex_buffer_recreate_00b492b0(*candidate->physical, device);
        if (FAILED(result)) return result;
        void* mapped = nullptr;
        result = states.lock_vertex_stream_00b49980(*candidate, payload.count, 0, false, mapped);
        if (FAILED(result)) return result;
        if (!mapped) {
            states.unlock_vertex_stream_00b49a80(*candidate);
            return E_FAIL;
        }
        std::memcpy(mapped, payload.bytes.data(), capacity);
        states.unlock_vertex_stream_00b49a80(*candidate);
        output = std::move(candidate);
        return S_OK;
    } catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }
}

HRESULT create_mesh_index_stream_00b4bf30_fragment(IDirect3DDevice9& device,
    D3D9StateCache& states, const MeshIndexPayload& payload,
    std::shared_ptr<LogicalIndexStream>& output) {
    const UINT width = payload.format == 0x65 ? 2u : payload.format == 0x66 ? 4u : 0u;
    UINT capacity = 0;
    if (payload.index_width != width
        || !payload_capacity(payload.count, width, payload.bytes.size(), capacity))
        return E_INVALIDARG;
    try {
        auto candidate = std::make_shared<LogicalIndexStream>();
        candidate->physical = std::make_shared<IndexBufferBinding>();
        candidate->index_count = payload.count;
        candidate->format = static_cast<D3DFORMAT>(payload.format);
        candidate->physical->flags = 1;
        candidate->physical->capacity = capacity;
        IDirect3DIndexBuffer9* created = nullptr;
        HRESULT result = device.CreateIndexBuffer(capacity, 0, candidate->format,
            D3DPOOL_MANAGED, &created, nullptr);
        // Transfer CreateIndexBuffer's reference. Native attach AddRefs then the
        // ctor releases its temporary; both leave exactly one owned reference.
        candidate->physical->buffer = created;
        if (FAILED(result)) return result;
        void* mapped = nullptr;
        result = states.lock_index_stream_00b49b60(*candidate, payload.count, 0, false, mapped);
        if (FAILED(result)) return result;
        if (!mapped) {
            states.unlock_index_stream_00b49c70(*candidate);
            return E_FAIL;
        }
        std::memcpy(mapped, payload.bytes.data(), capacity);
        states.unlock_index_stream_00b49c70(*candidate);
        // Native post-upload virtual +2Ch is the single RET at 00b49b30.
        output = std::move(candidate);
        return S_OK;
    } catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }
}
}
