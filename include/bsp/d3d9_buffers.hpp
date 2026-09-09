#pragma once
#include "bsp/d3d9_startup.hpp"
#include <cstdint>
#include <type_traits>
#include <vector>

namespace bsp {
struct LogicalVertexStream;
struct LogicalIndexStream;
// Semantic wrapper fields +14h..28h, without intrusive ownership/diagnostics.
// Owns one COM reference. Release explicitly for reset; new-interface destructor
// releases any remaining reference (not the complete native wrapper destructor).
template<class Buffer> struct D3D9BufferBinding {
    D3D9BufferBinding() = default;
    D3D9BufferBinding(const D3D9BufferBinding&) = delete;
    D3D9BufferBinding& operator=(const D3D9BufferBinding&) = delete;
    ~D3D9BufferBinding() { if (buffer) buffer->Release(); }
    using LogicalStream = std::conditional_t<std::is_same_v<Buffer, IDirect3DVertexBuffer9>,
        LogicalVertexStream, LogicalIndexStream>;
    // Non-owning registry (+8h/+Ch/+10h). Explicitly unregister before stream
    // destruction; full native constructors/destructors remain unported.
    std::vector<LogicalStream*> logical_streams;
    DWORD flags{};
    UINT capacity{};
    UINT cursor{};
    std::uint32_t lock_depth{};
    std::uint32_t dynamic_locks{};
    Buffer* buffer{};
};
using VertexBufferBinding = D3D9BufferBinding<IDirect3DVertexBuffer9>;
using IndexBufferBinding = D3D9BufferBinding<IDirect3DIndexBuffer9>;

HRESULT vertex_buffer_recreate_00b492b0(VertexBufferBinding&, IDirect3DDevice9&);
HRESULT index_buffer_recreate_00b49180(IndexBufferBinding&, IDirect3DDevice9&);

// Body phase of renderer00b237d0: caller supplies its optional renderer guard.
// Skip if not ready; otherwise clear ready before vertex then index COM drops.
// Preserve wrapper identities and all metadata; release has no lost-state gate.
void release_dynamic_buffers_for_reset_00b237d0_fragment(bool& ready,
    VertexBufferBinding& vertices, IndexBufferBinding& indices);

// Native renderer ECX, no stack arguments. No internal guard. Set ready before
// vertex then index recreation; index is attempted even when vertex fails.
// New HRESULT report: S_FALSE when ready/lost skips, otherwise first failure
// or final index result. Native ignores results and never rolls ready back.
// Stable wrappers/device required; full renderer layout/virtual dispatch omitted.
HRESULT restore_dynamic_buffers_00b1fd90(bool& ready, bool device_lost,
    VertexBufferBinding& vertices, IndexBufferBinding& indices, IDirect3DDevice9& device);

struct BufferLockResult { void* data{}; UINT base_offset{}; DWORD flags{}; };
// Native argument 3 is unused and omitted here. Partial port: null-buffer sentinel
// and diagnostic singleton paths return INVALIDCALL instead of executing them.
HRESULT vertex_buffer_lock_00b4ba00(VertexBufferBinding&, UINT bytes, UINT extra_offset,
    bool read_only, BufferLockResult&);
HRESULT index_buffer_lock_00b4b850(IndexBufferBinding&, UINT bytes, UINT extra_offset,
    bool read_only, BufferLockResult&);
void vertex_buffer_unlock_00b4b9d0(VertexBufferBinding&);
void index_buffer_unlock_00b4b820(IndexBufferBinding&);

template<class Buffer> void buffer_release(D3D9BufferBinding<Buffer>& binding) {
    if (binding.buffer) binding.buffer->Release();
    binding.buffer = nullptr; // Metadata survives; cursor is not implicitly rewound.
}
}
