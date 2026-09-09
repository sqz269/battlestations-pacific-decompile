#include "bsp/d3d9_buffers.hpp"

namespace bsp {
namespace {
bool decode_creation(DWORD flags, DWORD& usage, D3DPOOL& pool) {
    const DWORD native_pool = flags & 0xf;
    if (native_pool > 3) return false; // Native fallback misuses device pointer as pool.
    pool = static_cast<D3DPOOL>(native_pool);
    if (native_pool == 0) flags |= 0x10000;
    usage = (flags & 0x10) ? 1u : 0u;
    switch (flags & 0xf00) {
    case 0x100: usage |= 2; break;
    case 0x200: usage |= 0x4000; break;
    case 0x300: usage |= 0x40; break;
    case 0x400: usage |= 0x100; break;
    case 0x500: usage |= 0x80; break;
    }
    if ((flags & 0xf000) == 0x1000) usage |= D3DUSAGE_DYNAMIC;
    if ((flags & 0xf0000) == 0x10000) usage |= D3DUSAGE_WRITEONLY;
    return true;
}

template<class Buffer> void install(D3D9BufferBinding<Buffer>& binding, Buffer* created) {
    Buffer* previous = binding.buffer;
    if (previous != created) {
        binding.buffer = created;
        if (created) created->AddRef();
        if (previous) previous->Release();
    }
    if (created) created->Release(); // Drop CreateBuffer's temporary reference.
}

template<class Buffer> HRESULT lock(D3D9BufferBinding<Buffer>& binding, UINT bytes,
    UINT extra_offset, bool read_only, BufferLockResult& output) {
    output = {};
    // Guard unsupported singleton/sentinel paths and integer overflow in the new API.
    // The native capacity diagnostic checks cursor+offset, not the requested size.
    if (!binding.buffer || extra_offset > binding.capacity
        || binding.cursor > binding.capacity - extra_offset
        || ((binding.flags & 0x1000) && extra_offset != 0)) return D3DERR_INVALIDCALL;
    output.flags = D3DLOCK_NOSYSLOCK | (read_only ? D3DLOCK_READONLY : 0);
    if (binding.flags & 0x1000) {
        output.flags = binding.cursor == 0 ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE;
        ++binding.dynamic_locks;
    }
    const HRESULT result = binding.buffer->Lock(binding.cursor + extra_offset, bytes,
        &output.data, output.flags);
    output.base_offset = binding.cursor; // Deliberately excludes extra_offset.
    if (binding.flags & 0x1000) binding.cursor += bytes + extra_offset;
    ++binding.lock_depth; // Original advances metadata even when Lock fails.
    return result;
}

template<class Buffer> void unlock(D3D9BufferBinding<Buffer>& binding) {
    if (binding.buffer) {
        binding.buffer->Unlock();
        --binding.lock_depth;
    }
}

template<class Buffer> void release_for_reset(D3D9BufferBinding<Buffer>& binding) {
    // Native reads the member again after the balanced COM access pair.
    Buffer* observed = binding.buffer;
    if (observed) {
        observed->AddRef();
        observed->Release();
    }
    if (binding.buffer) {
        binding.buffer->Release();
        binding.buffer = nullptr;
    }
}
}

HRESULT vertex_buffer_recreate_00b492b0(VertexBufferBinding& binding, IDirect3DDevice9& device) {
    DWORD usage; D3DPOOL pool;
    if (!decode_creation(binding.flags, usage, pool)) return D3DERR_INVALIDCALL;
    IDirect3DVertexBuffer9* created = nullptr;
    const HRESULT result = device.CreateVertexBuffer(binding.capacity, usage, 0, pool, &created, nullptr);
    if (SUCCEEDED(result)) install(binding, created); // Preserve old owner on API failure.
    return result;
}

HRESULT index_buffer_recreate_00b49180(IndexBufferBinding& binding, IDirect3DDevice9& device) {
    DWORD usage; D3DPOOL pool;
    if (!decode_creation(binding.flags, usage, pool)) return D3DERR_INVALIDCALL;
    IDirect3DIndexBuffer9* created = nullptr;
    const HRESULT result = device.CreateIndexBuffer(binding.capacity, usage, D3DFMT_INDEX16, pool, &created, nullptr);
    if (SUCCEEDED(result)) install(binding, created);
    return result;
}

void release_dynamic_buffers_for_reset_00b237d0_fragment(bool& ready,
    VertexBufferBinding& vertices, IndexBufferBinding& indices) {
    if (!ready) return;
    ready = false;
    release_for_reset(vertices);
    release_for_reset(indices);
}

HRESULT restore_dynamic_buffers_00b1fd90(bool& ready, bool device_lost,
    VertexBufferBinding& vertices, IndexBufferBinding& indices, IDirect3DDevice9& device) {
    if (ready || device_lost) return S_FALSE;
    ready = true;
    const HRESULT vertex_result = vertex_buffer_recreate_00b492b0(vertices, device);
    const HRESULT index_result = index_buffer_recreate_00b49180(indices, device);
    return FAILED(vertex_result) ? vertex_result : index_result;
}

HRESULT vertex_buffer_lock_00b4ba00(VertexBufferBinding& b, UINT bytes, UINT offset,
    bool read_only, BufferLockResult& out) { return lock(b, bytes, offset, read_only, out); }
HRESULT index_buffer_lock_00b4b850(IndexBufferBinding& b, UINT bytes, UINT offset,
    bool read_only, BufferLockResult& out) { return lock(b, bytes, offset, read_only, out); }
void vertex_buffer_unlock_00b4b9d0(VertexBufferBinding& b) { unlock(b); }
void index_buffer_unlock_00b4b820(IndexBufferBinding& b) { unlock(b); }
}
